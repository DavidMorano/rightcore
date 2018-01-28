# MAKEFILE

T= liblkcmd

ALL= $(T).so $(T).x

SRCROOT= $(LOCAL)

EXTRA= /usr/extra
PCS= /usr/add-on/pcs


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

ABINDIR= $(AST)/bin
AINCDIR= $(AST)/include/ast
ALIBDIR= $(AST)/lib
AMANDIR= $(AST)/man/man1
AHELPDIR= $(AST)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS_GCCOPT= -O3 $(CCOPTS_GCCOPTSPARC)
#CCOPTS_GCCOPT= -g -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS_GCCOPT= -g -pg $(CCOPTS_GCCOPTSPARC)
#CCOPTS_GCCOPT= -g -Wstrict-aliasing $(CCOPTS_GCCOPTSPARC)
#CCOPTS_GCCOPT= -g -Wall $(CCOPTS_GCCOPTSPARC)

CCOPTS_GCCLIB= -fPIC $(CCOPTS_GCCLIBSPARC)

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
#CCOPTS= -xO5 -K PIC -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -K PIC -xtarget=ultra -xsafe=mem -xdepend


# OK for AST and non-AST modules
DEF00=
DEF01=
DEF02=
DEF03=
DEF04=
DEF05=
DEF06= 
DEF07= $(LF_DEFS)

# OK only for non-AST objects modules
DEF08=
DEF09= 
DEF10=
DEF11=
DEF12=
DEF13=
DEF14=
DEF15=

# OK only for AST modules
DEF16=
DEF17= -DKSHBUILTIN=1 -DSFIO=1
DEF18=
DEF19=
DEF20=
DEF21=
DEF22=
DEF23=

DEFA= $(DEF00) $(DEF01) $(DEF02) $(DEF03) $(DEF04) $(DEF05) $(DEF06) $(DEF07)
DEFB= $(DEF08) $(DEF09) $(DEF10) $(DEF11) $(DEF12) $(DEF13) $(DEF14) $(DEF15)
DEFC= $(DEF16) $(DEF17) $(DEF18) $(DEF19) $(DEF20) $(DEF21) $(DEF22) $(DEF23)

DEFS= $(DEFA) $(DEFB)

# special for AST dependent softare
ASTDEFS= $(DEFA) $(DEFC)

# special for PCS dependent softare
PCSDEFS= $(DEFA) $(DEFB)

# special for MFS dependent softare
MFSDEFS= $(DEFA) $(DEFB)

# special for AST+PCS dependent softare
ASTPCSDEFS= $(DEFA) $(DEFC)

# special for AST+MFS dependent softare
ASTMFSDEFS= $(DEFA) $(DEFC)


GNUINCDIRS= -I$(GNU)/include

INCDIRS= -I$(INCDIR)

ASTINCDIRS= -I$(AINCDIR) -I$(INCDIR) 

PCSINCDIRS= -I$(PCS)/include -I$(INCDIR)

MFSINCDIRS= -I$(PCS)/include -I$(INCDIR)

ASTPCSINCDIRS= -I$(AINCDIR) -I$(PCS)/include -I$(INCDIR) 

ASTMFSINCDIRS= -I$(AINCDIR) -I$(PCS)/include -I$(INCDIR) 


CPPFLAGS= $(DEFS) $(INCDIRS)

# AST 
ASTCPPFLAGS= $(ASTDEFS) $(ASTINCDIRS)

# PCS
PCSCPPFLAGS= $(PCSDEFS) $(PCSINCDIRS)

# MFS
MFSCPPFLAGS= $(MFSDEFS) $(MFSINCDIRS)

# AST-PCS
ASTPCSCPPFLAGS= $(ASTPCSDEFS) $(ASTPCSINCDIRS)

# AST-MFS
ASTMFSCPPFLAGS= $(ASTMFSDEFS) $(ASTMFSINCDIRS)


CFLAGS= $(CCOPTS) 
CCFLAGS= $(GPPFLAGS) $(CCOPTS) 

ASTCFLAGS= $(CCOPTS)

PCSCFLAGS= $(CCOPTS)

MFSCFLAGS= $(CCOPTS)

ASTPCSCFLAGS= $(CCOPTS)

ASTMFSCFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= ld

LDRPATH= $(EXTRA)/lib:/usr/lib/lwp

LDFLAGS= -m -z ignore $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
LDFLAGS_SO= -m -z ignore $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
LDFLAGS_R= $(LF_LDFLAGS)


ALIBDIRS= -L$(AST)/lib

ALIBS= -lshell -last


DLIBDIRS= -L$(PCS)/lib -L$(LIBDIR) 

DLIB0=
DLIB1= -lpcs -lcsdb -ldam -lb -lut
DLIB2=
DLIB3= -luc -lu
DLIB4=
DLIB5=
DLIB6=
DLIB7=

DLIBS= $(DLIB0) $(DLIB1) $(DLIB2) $(DLIB3) $(DLIB4) $(DLIB5) $(DLIB6) $(DLIB7)


GLIBDIRS= -L$(CGS_LIBDIR)

GLIBS= -liconv $(CGS_LIBS)


ULIBDIRS= -L/usr/lib

ULIB0=
ULIB1=
ULIB2=
ULIB3=
ULIB4=
ULIB5= -lsec 
ULIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
ULIB7=

ULIBA= $(ULIB0) $(ULIB1) $(ULIB2) $(ULIB3)
ULIBB= $(ULIB4) $(ULIB5) $(ULIB6) $(ULIB7)

ULIBS= $(ULIBA) $(ULIBB)


SLIBDIRS= $(ALIBDIRS) $(DLIBDIRS) $(GLIBDIRS)

SLIBS= $(ALIBS) $(DLIBS) $(GLIBS) $(ULIBS)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= $(DLIBDIRS) $(GLIBDIRS)

XLIBS= -llkcmd $(DLIBS) $(GLIBS) $(ULIBS) -ldl -lc

XLIBINFO= $(XLIBDIRS) $(XLIBS)


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

CRT0= $(CRT1) $(CRTI) $(VALUES)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -ux -n -Dlint

NM= nm
NMFLAGS= -sx -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /usr/bin/touch


INSTALLINCS0= install.inca install.incb install.incc install.incd
INSTALLINCS1= install.ince install.incf

INSTALLINCS= $(INSTALLINC0) $(INSTALLINC1)


I00=
I01= defs.h
I02= shio.h
I03= kshlib.h
I04=
I05=
I06=
I07=
I08=
I09=
I10=
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

INCS= $(INCA) $(INCB) $(INCC) $(INCD) $(INCE) $(INCF)


MSUINC0= msumain.h msumainer.h msulocinfo.h msuconfig.h msuadj.h 
MSUINC1= msulog.h msumsg.h

MSUINCS= $(MSUINC0) $(MSUINC1)


PCSINC0= pcsmain.h pcslocinfo.h pcsconfig.h pcsadj.h
PCSINC1= pcslog.h pcsmsg.h

PCSINCS= $(PCSINC0) $(PCSINC1)


MFSINC0= mfsmain.h mfslocinfo.h mfsconfig.h mfsadj.h
MFSINC1= mfswatch.o mfslog.h mfsmsg.h
MFSINC2= sreqdb.h sreq.h svcentsub.h
MFSINC3=

MFSINCS= $(MFSINC0) $(MFSINC1) $(MSGINC2) $(MFSINC3)


OBJ00= b_la.o b_msu.o b_msinfo.o b_wn.o b_touch.o b_rename.o b_mkincfile.o
OBJ01= b_l.o b_shcat.o b_imail.o b_holiday.o b_numcvt.o b_fsinfo.o b_chunk.o
OBJ02= b_commandment.o b_biblebook.o b_bibleverse.o b_bibleq.o b_logadm.o
OBJ03= b_clustername.o b_s.o b_kshbi.o b_chacl.o b_smesg.o b_syncer.o 
OBJ04= b_fileolder.o b_pathenum.o b_userinfo.o b_groupinfo.o b_projectinfo.o
OBJ05= b_cutter.o b_scanbad.o b_loghist.o b_pt.o b_pathclean.o b_rmshm.o
OBJ06= b_mailalias.o b_sanity.o b_ismailaddr.o b_babies.o b_inproject.o
OBJ07= b_sysvar.o b_mxalias.o b_issue.o b_motd.o b_logfile.o b_username.o 

OBJ08= b_querystring.o b_loginblurb.o b_isfile.o b_cal.o b_mkwords.o
OBJ09= b_fibonacci.o b_factorial.o b_makenewer.o b_sysval.o b_temp.o b_lastlog.o
OBJ10= b_netload.o b_dhtext.o b_logsys.o b_termadm.o b_mailnew.o b_cookieset.o
OBJ11= b_pcsconf.o b_pcsuserinfo.o b_inetping.o b_linefold.o b_summer.o
OBJ12= b_conslog.o b_termnote.o b_uuidgen.o b_strerror.o b_sysdb.o 
OBJ13= b_rest.o b_qotd.o b_mjd.o b_look.o b_append.o b_rfinger.o b_shells.o
OBJ14= b_mbproc.o b_mbexpire.o b_homepage.o b_progcheck.o b_calyear.o b_hello.o
OBJ15= b_isproc.o b_webcounter.o b_makesafe.o b_numbers.o b_cksumpass.o

OBJ16= b_fieldwords.o b_prtdb.o
OBJ17= 
OBJ18= 
OBJ19= 
OBJ20= 
OBJ21=
OBJ22=
OBJ23=

OBJ24= sfreadline.o sfisterm.o 
OBJ25= printhelp.o
OBJ26= shio.o shio_printdline.o shio_writeblanks.o
OBJ27= cgi.o htm.o
OBJ28= svckv.o svcent.o
OBJ29= 
OBJ30= 
OBJ31= kshlib.o

OBJ32= mailbox.o mailbox_getfrom.o mailbox_fromaddr.o mailmsgstage.o
OBJ33= mailmsg.o mailmsg_loadmb.o mailmsg_enver.o
OBJ34= mailmsghdrval.o mailmsghdrfold.o 
OBJ35= mailmsghdrs.o
OBJ36= mailalias.o lastlogfile.o
OBJ37= cachetime.o pdb.o
OBJ38= vecstr_avmkstr.o
OBJ39= field_word.o field_wordphrase.o 

OBJ40= vecpstr_loadgrusers.o vecpstr_loadpjusers.o
OBJ41= hostinfo.o finduid.o pwcache.o spawner.o ema.o ema_first.o
OBJ42= cm.o cmbuf.o sysdialer.o systems.o
OBJ43= logfile.o logfile_userinfo.o logfile_printfold.o
OBJ44= mbcache.o conslog.o sigblock.o
OBJ45= aclinfo.o acltypes.o
OBJ46= pwi.o ipasswd.o 
OBJ47= lookword.o mxalias.o issue.o motd.o

OBJ48= osetstr.o vecint.o raqhand.o
OBJ49= sysusernames.o sysrealname.o
OBJ50= sysusers.o sysgroups.o sysprojects.o sysusershells.o
OBJ51= sysuserattr.o sysgroup.o sysproject.o
OBJ52=
OBJ53=
OBJ54= hdrdecode.o qpdecoder.o b64decoder.o hexdecoder.o obuf.o 
OBJ55= chartrans.o utf8decoder.o

OBJ56= filebuf.o filebuf_mailsupport.o
OBJ57= outstore.o calyear.o numincr.o modload.o uiconv.o
OBJ58= sesmsg.o msgdata.o
OBJ59= msfile.o msfile_best.o msfilee.o ebuf.o
OBJ60= sysvar.o var.o varmk.o
OBJ61= namecache.o gncache.o msumsg.o termout.o linehist.o langstate.o
OBJ62= sbuf.o sbuf_loadstrs.o
OBJ63=

OBJ64= initnow.o inittimezone.o spawnproc.o logout.o 
OBJ65= pwilookup.o mailboxappend.o mailmsgfrom.o
OBJ66= ndig.o address.o 
OBJ67= mkquoted.o mkfingerquery.o mkcleanline.o mkuuid.o
OBJ68= mailmsgmatenv.o mailmsgmathdr.o
OBJ69= getopendial.o getfstype.o getuserorg.o getrealname.o
OBJ70=
OBJ71= isOneOf.o isIOError.o isValidMagic.o iseol.o

OBJ72= mkdisplayable.o compactstr.o
OBJ73= mkmid.o
OBJ74= fileobject.o filebinary.o 
OBJ75= 
OBJ76=
OBJ77=
OBJ78=
OBJ79= percache.o perdata.o

OBJ80= snwcpyopaque.o snwcpycompact.o snwcpyclean.o snmkuuid.o
OBJ81= snopenflags.o snflags.o
OBJ82= sfword.o 
OBJ83=
OBJ84=
OBJ85= ctdecp.o
OBJ86= strval.o
#OBJ87= debug.o debugprintf.o nprintf.o strlinelen.o

OBJ88= pcsmailcheck.o pcsinfoset.o
OBJ89=
OBJ90= localgetorg.o localgetorgcode.o localgetorgloc.o
OBJ91= localgetnetload.o localgetsystat.o
OBJ92= prmktmpdir.o
OBJ93= mkdirs.o rmsesfiles.o isdirempty.o
OBJ94=
OBJ95= maintqotd.o maintqotd_prog.o 

OBJ096= proginfo.o proginfo_setpiv.o proginfo_rootname.o 
OBJ097= progdebugout.o proglog.o proglogout.o proguserlist.o
OBJ098= maininfo.o sysmemutil.o
OBJ099= 
OBJ100= msghdr.o cmsghdr.o
OBJ101= getourenv.o
OBJ102= getlogname.o getmjd.o getnodeinfo.o getclusters.o 
OBJ103= hdrextnum.o lastlogin.o termwritable.o opendial.o openqotd.o


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

OBJMa= $(OBJ096) $(OBJ097) $(OBJ098) $(OBJ099) 
OBJMb= $(OBJ100) $(OBJ101) $(OBJ102) $(OBJ103)
OBJM= $(OBJMa) $(OBJMb)

OBJa= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) $(OBJI)
OBJb= $(OBJJ) $(OBJK) $(OBJL) $(OBJM)

OBJ= $(OBJa) $(OBJb)

OBJSa= objast.o obje.o objf.o objg.o objh.o 
OBJSb= obji.o objj.o objk.o objl.o objm.o

OBJS= $(OBJSa) $(OBJSb)


ASTOBJS= $(OBJA) $(OBJB) $(OBJC) $(OBJD)


MSUOBJS= msumain.o msulocinfo.o msuconfig.o msuadj.o msulog.o msudebug.o


PCSOBJS= pcsmain.o pcslocinfo.o pcsconfig.o pcsadj.o pcslog.o pcsdebug.o


MFSOBJ00= mfsmain.o mfslocinfo.o mfsconfig.o mfsadj.o mfslog.o mfsdebug.o
MFSOBJ02=
MFSOBJ03= sreqdb.o sreq.o svcentsub.o
MFSOBJ04=

MFSOBJS= $(MFSOBJ00) $(MFSOBJ01) $(MFSOBJ02) $(MFSOBJ03)


XOBJ= main.o maininfo.o sighand.o ucontext_rtn.o

XOBJS= $(CRTFRONT) $(XOBJ) $(CRTC) $(CRTBACK)


.SUFFIXES:		.ls .i .cx .cs .ai


.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(ASTCPPFLAGS) $<

# stand-alone LINT analysis
.c.ls:
	$(LINT) $(LINTFLAGS) $(ASTCPPFLAGS) $(GNUINCDIRS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $(GNUINCDIRS) $< > $(*).i

.c.ai:
	$(CPP) $(ASTCPPFLAGS) $(GNUINCDIRS) $< > $(*).ai

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $(GNUINCDIRS) $<

# stand-alone cross reference
.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) $(GNUINCDIRS) -o $(*).cs $<


default:		$(ALL)

so:			$(T).so

x:			$(T).x

a:			$(T).a

$(T).a:			$(OBJ)
	$(AR) -cr $(T).a $?

$(T).so:		$(OBJS) Makefile $(T).map
	$(LD) -G -o $@ -M $(T).map $(LDFLAGS_SO) $(OBJS) $(SLIBINFO) > $(T).lm

$(T).x:			$(XOBJ) Makefile $(T).so
	makedate -m $(T) > makedate.c
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(XOBJS) $(XLIBINFO) > main.lm

$(T).nm nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

safe:
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(ASTOBJS)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJA)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJB)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJC)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJD)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJE)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJF)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJG)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJH)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJI)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJJ)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJK)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJL)
	makesafe -v=3 -I $(INCDIR) -I $(AST)/include/ast $(OBJM)

strip:			$(T).x
	strip $(T).x

install:		install-so install-prog

install-prog:		$(T).x
	rm -f $(T)
	ln -f $(T).x $(T)
	bsdinstall $(T) $(BINDIR)

install-a:		$(T).a Makefile 
	makenewer -r $(T).a $(LIBDIR)

install-so:		$(T).so Makefile 
	bsdinstall $(T).so $(LIBDIR)

install-help:
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf *.help -d $(HELPDIR)

other:			here
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
	makenewer -im -z -m o=h $(OBJM) -d $(INCDIR) -t $@

again:
	$(RM) $(ALL) $(T).lm

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	(uname -n ; date) > Control


objast.o:		$(ASTOBJS) Makefile
	$(LD) -r -o $@ -M $(T).map $(LDFLAGS_R) $(ASTOBJS)

obje.o:			$(OBJE) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJE)

objf.o:			$(OBJF) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJF)

objg.o:			$(OBJG) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJG)

objh.o:			$(OBJH) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJH)

obji.o:			$(OBJI) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJI)

objj.o:			$(OBJJ) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJJ)

objk.o:			$(OBJK) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJK)

objl.o:			$(OBJL) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJL)

objm.o:			$(OBJM) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJM)


# special partial links

b_msu.o:		$(MSUOBJS) Makefile
	$(LD) -r -o $@ -M msu.map -B reduce $(LDFLAGS_R) $(MSUOBJS)

b_pcs.o:		$(PCSOBJS) Makefile
	$(LD) -r -o $@ -M pcs.map -B reduce $(LDFLAGS_R) $(PCSOBJS)

b_mfserve.o:		$(MFSOBJS) Makefile
	$(LD) -r -o $@ -M mfs.map -B reduce $(LDFLAGS_R) $(MFSOBJS)


# regular compiles

b_la.o:			b_la.c b_la.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_msinfo.o:		b_msinfo.c b_msinfo.h $(INCS) msfile.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_wn.o:			b_wn.c b_wn.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_rename.o:		b_rename.c b_rename.h $(INCS) numincr.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_shcat.o:		b_shcat.c b_shcat.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_touch.o:		b_touch.c b_touch.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_commandment.o:	b_commandment.c b_commandment.h $(INCS) commandments.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_biblebook.o:		b_biblebook.c b_biblebook.h $(INCS) biblebooks.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_bibleverse.o:		b_bibleverse.c b_bibleverse.h $(INCS) bibleverses.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_bibleq.o:		b_bibleq.c b_bibleq.h $(INCS) bibleq.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_l.o:			b_l.c b_l.h $(INCS) fsdir.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_clustername.o:	b_clustername.c b_clustername.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_s.o:			b_s.c b_s.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_chacl.o:		b_chacl.c b_chacl.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_kshbi.o:		b_kshbi.c b_kshbi.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_syncer.o:		b_syncer.c b_syncer.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_fileolder.o:		b_fileolder.c b_fileolder.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_pathenum.o:		b_pathenum.c b_pathenum.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_groupinfo.o:		b_groupinfo.c b_groupinfo.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_projectinfo.o:	b_projectinfo.c b_projectinfo.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_smesg.o:		b_smesg.c b_smesg.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_cutter.o:		b_cutter.c b_cutter.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_scanbad.o:		b_scanbad.c b_scanbad.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_loghist.o:		b_loghist.c b_loghist.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_pathclean.o:		b_pathclean.c b_pathclean.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mailalias.o:		b_mailalias.c b_mailalias.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_sanity.o:		b_sanity.c b_sanity.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_ismailaddr.o:		b_ismailaddr.c b_ismailaddr.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_babies.o:		b_babies.c b_babies.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_inproject.o:		b_inproject.c b_inproject.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_sysvar.o:		b_sysvar.c b_sysvar.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mxalias.o:		b_mxalias.c b_mxalias.h $(INCS) mxalias.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_issue.o:		b_issue.c b_issue.h $(INCS) issue.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_motd.o:		b_motd.c b_motd.h $(INCS) motd.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_holiday.o:		b_holiday.c b_holiday.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_calyear.o:		b_calyear.c b_calyear.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_logfile.o:		b_logfile.c b_logfile.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_querystring.o:	b_querystring.c b_querystring.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_rmshm.o:		b_rmshm.c b_rmshm.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_fsinfo.o:		b_fsinfo.c b_fsinfo.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_logadm.o:		b_logadm.c b_logadm.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_loginblurb.o:		b_loginblurb.c b_loginblurb.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_isfile.o:		b_isfile.c b_isfile.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_cal.o:		b_cal.c b_cal.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mkwords.o:		b_mkwords.c b_mkwords.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_fibonacci.o:		b_fibonacci.c b_fibonacci.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_factorial.o:		b_factorial.c b_factorial.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_makenewer.o:		b_makenewer.c b_makenewer.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_sysval.o:		b_sysval.c b_sysval.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_netload.o:		b_netload.c b_netload.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_rfinger.o:		b_rfinger.c b_rfinger.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_dhtext.o:		b_dhtext.c b_dhtext.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_logsys.o:		b_logsys.c b_logsys.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_termadm.o:		b_termadm.c b_termadm.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mailnew.o:		b_mailnew.c b_mailnew.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_conslog.o:		b_conslog.c b_conslog.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_termnote.o:		b_termnote.c b_termnote.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_uuidgen.o:		b_uuidgen.c b_uuidgen.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_qotd.o:		b_qotd.c b_qotd.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mjd.o:		b_mjd.c b_mjd.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_look.o:		b_look.c b_look.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_append.o:		b_append.c b_append.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_shells.o:		b_shells.c b_shells.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_temp.o:		b_temp.c b_temp.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_lastlog.o:		b_lastlog.c b_lastlog.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mailexpire.o:		b_mailexpire.c b_mailexpire.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mbexpire.o:		b_mbexpire.c b_mbexpire.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mbproc.o:		b_mbproc.c b_mbproc.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_strerror.o:		b_strerror.c b_strerror.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_sysdb.o:		b_sysdb.c b_sysdb.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_homepage.o:		b_homepage.c b_homepage.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_progcheck.o:		b_progcheck.c b_progcheck.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_inetping.o:		b_inetping.c b_inetping.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_inetping.ai:		b_inetping.c b_inetping.h $(INCS)
	$(CPP) $(ASTCPPFLAGS) $(GNUINCDIRS) b_inetping.c > b_inetping.ai

b_linefold.o:		b_linefold.c b_linefold.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_isproc.o:		b_isproc.c b_isproc.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_webcounter.o:		b_webcounter.c b_webcounter.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_makesafe.o:		b_makesafe.c b_makesafe.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_summer.o:		b_summer.c b_summer.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_mkincfile.o:		b_mkincfile.c b_mkincfile.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_cookieset.o:		b_cookieset.c b_cookieset.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_chunk.o:		b_chunk.c b_chunk.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_fieldwords.o:		b_fieldwords.c b_fieldwords.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_prtdb.o:		b_prtdb.c b_prtdb.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_hello.o:		b_hello.c b_hello.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<


# special PCS (or pseudo PCS)

b_imail.o:		b_imail.c b_imail.h $(INCS) outema.h
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<

b_username.o:		b_username.c b_username.h $(INCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<

b_userinfo.o:		b_userinfo.c b_userinfo.h $(INCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<

b_pcsconf.o:		b_pcsconf.c b_pcsconf.h $(INCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<

b_pcsuserinfo.o:	b_pcsuserinfo.c b_pcsuserinfo.h $(INCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<

b_rest.o:		b_rest.c b_rest.h $(INCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<


# MSU code
msu:			b_msu.o
msulocinfo.o:		msulocinfo.c msulocinfo.h $(INCS) $(MSUINCS)
msuconfig.o:		msuconfig.c msulocinfo.h $(INCS) $(MSUINCS)
msuadj.o:		msuadj.c msuadj.h $(INCS) $(MSUINCS)
msulog.o:		msulog.c msulog.h $(INCS) $(MSUINCS)
msudebug.o:		msudebug.c msulog.h $(INCS) $(MSUINCS)
# special since it needs AST
msumain.o:		msumain.c $(INCS) msfile.h $(MSUINCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<


# PCS code
pcs:			b_pcs.o
pcslocinfo.o:		pcslocinfo.c pcslocinfo.h $(INCS) $(PCSINCS)
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<
pcsconfig.o:		pcsconfig.c pcslocinfo.h $(INCS) $(PCSINCS)
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<
pcsadj.o:		pcsadj.c pcsadj.h $(INCS) $(PCSINCS)
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<
pcslog.o:		pcslog.c pcslog.h $(INCS) $(PCSINCS)
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<
pcsdebug.o:		pcsdebug.c pcslog.h $(INCS) $(PCSINCS)
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<
# special since it needs AST
pcsmain.o:		pcsmain.c $(INCS) msfile.h $(PCSINCS)
	$(CC) -c $(ASTPCSCFLAGS) $(ASTPCSCPPFLAGS) $<


# MFS code
mfs:			b_mfserve.o
mfslocinfo.o:		mfslocinfo.c mfslocinfo.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
mfsconfig.o:		mfsconfig.c mfslocinfo.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
mfsadj.o:		mfsadj.c mfsadj.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
mfslog.o:		mfslog.c mfslog.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
mfsdebug.o:		mfsdebug.c mfslog.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
sreqdb.o:		sreqdb.c sreqdb.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
sreq.o:			sreq.c sreq.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
svcentsub.o:		svcentsub.c svcentsub.h $(INCS) $(MFSINCS)
	$(CC) -c $(MFSCFLAGS) $(MFSCPPFLAGS) $<
# special since it needs AST
mfsmain.o:		mfsmain.c mfsmain.h $(INCS) $(MFSINCS)
	$(CC) -c $(ASTMFSCFLAGS) $(ASTMFSCPPFLAGS) $<


# put the C++ bullt-ins here

b_numbers.o:		b_numbers.c b_numbers.h $(INCS)
	$(GPP) -c $(GPPFLAGS) $(ASTCFLAGS) $(ASTCPPFLAGS) b_numbers.cc

b_numcvt.o:		b_numcvt.c b_numcvt.h $(INCS)
	$(GPP) -c $(GPPFLAGS) $(ASTCFLAGS) $(ASTCPPFLAGS) b_numcvt.cc

b_cksumpass.o:		b_cksumpass.c b_cksumpass.h $(INCS)
	$(GPP) -c $(GPPFLAGS) $(ASTCFLAGS) $(ASTCPPFLAGS) b_cksumpass.cc

b_pt.o:			b_pt.cc b_pt.h $(INCS)
	$(GPP) -c $(GPPFLAGS) $(ASTCFLAGS) $(ASTCPPFLAGS) b_pt.cc


# testing

b_null.o:		b_null.c nulb_l.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testmail.o:		b_testmail.c b_testmail.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testcmd.o:		b_testcmd.c b_testcmd.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testenv.o:		b_testenv.c b_testenv.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testlock.o:		b_testlock.c b_testlock.h $(INCS) dumper.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testcat.o:		b_testcat.c b_testcat.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testsfio2.o:		b_testsfio2.c b_testsfio2.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<


# normal library compile begin

main.o:			main.c maininfo.h kshlib.h sesmsg.h msgdata.h
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

maininfo.o:		maininfo.c maininfo.h kshlib.h
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

sesmsg.o:		sesmsg.c sesmsg.h
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

msgdata.o:		msgdata.c msgdata.h
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

sighand.o:		sighand.c sighand.h
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

ucontext_rtn.o:		ucontext_rtn.c
	$(CC) -c $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CPPFLAGS) $<

# normal library compile end


# AST library compile begin

sfreadline.o:		sfreadline.c
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

sfisterm.o:		sfisterm.c
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

sfreadline.ls:	sfreadline.c
	$(LINT) $(LINTFLAGS) $(ASTCPPFLAGS) $(ASTINCDIRS) $(*).c

sfisterm.ls:	sfisterm.c
	$(LINT) $(LINTFLAGS) $(ASTCPPFLAGS) $(ASTINCDIRS) $(*).c

printhelp.o:		printhelp.c shio.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

kshlib.o:		kshlib.c $(INCS) sesmsg.h msgdata.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

shio.o:			shio.c shio.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

shio_writeblanks.o:	shio_writeblanks.c shio.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

shio_printdline.o:	shio_printdline.c shio.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

cgi.o:			cgi.c cgi.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c

htm.o:			htm.c htm.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $(*).c


# AST library compile end


# PCS regulars

pcsmailcheck.o:		pcsmailcheck.c
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsgetprogpath.o:	pcsgetprogpath.c
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsinfoset.o:		pcsinfoset.c
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsgetorg.o:		pcsgetorg.c
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsgetfacility.o:	pcsgetfacility.c
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

# PCS objects

pcsns.o:		pcsns.c pcsns.h
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsnsmgr.o:		pcsnsmgr.c pcsnsmgr.h
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsnsrecs.o:		pcsnsrecs.c pcsnsrecs.h
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<

pcsnsc.o:		pcsnsc.c pcsnsc.h
	$(CC) -c $(PCSCFLAGS) $(PCSCPPFLAGS) $<


# regular

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

progdebugout.o:		progdebugout.c defs.h

proglogout.o:		proglogout.c defs.h

proglogid.o:		proglogid.c defs.h config.h

proguserlist.o:		proguserlist.c defs.h config.h

proglog.o:		proglog.c defs.h config.h


perdata.o:		perdata.c percache.h

percache.o:		percache.c percache.h

getbufsize.o:		getbufsize.c getbufsize.h

getxusername.o:		getxusername.c getxusername.h

spawnproc.o:		spawnproc.c spawnproc.h

upt.o:			upt.c upt.h

pwilookup.o:		pwilookup.c pwi.h ipasswd.h

lastlogin.o:		lastlogin.c lastlogfile.h

openqotd.o:		openqotd.c openqotd.h

opendial.o:		opendial.c opendial.h

getopendial.o:		getopendial.c opendial.h

ctdecpi.o:		ctdecpi.c

getrootdname.o:		getrootdname.c

getlogname.o:		getlogname.c

getclusters.o:		getclusters.c

getopendial.o:		getopendial.c opendial.h

termwritable.o:		termwritable.c

mkcleanline.o:		mkcleanline.c

mkuuid.o:		mkuuid.c mkuuid.h

snmkuuid.o:		snmkuuid.c snmkuuid.h mkuuid.h

maintqotd.o:		maintqotd.c maintqotd.h

maintqotd_prog.o:	maintqotd_prog.c maintqotd.h

svckv.o:		svckv.c svckv.h

svcent.o:		svcent.c svcent.h


# objects

paramfile.o:		paramfile.c paramfile.h

paramopt.o:		paramopt.c paramopt.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

char.o:			char.c char.h

field.o:		field.c field.h

field_word.o:		field_word.c field.h

field_wordphrase.o:	field_wordphrase.c field.h

field_srvarg.o:		field_srvarg.c field.h

modload.o:		modload.c modload.h

filecounts.o:		filecounts.c filecounts.h

fifostr.o:		fifostr.c fifostr.h

fifoelem.o:		fifoelem.c fifoelem.h

vecstr.o:		vecstr.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_envfile.o:	vecstr_envfile.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_avmkstr.o:	vecstr_avmkstr.c vecstr.h

keyopt.o:		keyopt.c keyopt.h

vecelem.o:		vecelem.c vecelem.h

veclist.o:		vechand.c vechand.h

hdb.o:			hdb.c hdb.h

loadave.o:		loadave.c loadave.h

entropy.o:		entropy.c entropy.h

sha1.o:			sha1.c sha1.h

buffer.o:		buffer.c buffer.h

buffer_stropaque.o:	buffer_stropaque.c buffer.h

buffer_strquote.o:	buffer_strquote.c buffer.h

bufstr.o:		bufstr.c bufstr.h

sbuf.o:			sbuf.c sbuf.h

sbuf_loadstrs.o:	sbuf_loadstrs.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

sbuf_termconseq.o:	sbuf_termconseq.c sbuf.h

sbuf_blanks.o:		sbuf_blanks.c sbuf.h

lookaside.o:		lookaside.c lookaside.h

plainq.o:		plainq.c plainq.h

filemap.o:		filemap.c filemap.h

q.o:			q.c q.h

plainq.o:		plainq.c plainq.h

pq.o:			pq.c pq.h

charq.o:		charq.c charq.h

serialbuf.o:		serialbuf.c serialbuf.h

netorder.o:		netorder.c netorder.h

storeitem.o:		storeitem.c storeitem.h

outbuf.o:		outbuf.c outbuf.h

mailbox.o:		mailbox.c mailbox.h mailmsg.h

mailbox_getfrom.o:	mailbox_getfrom.c mailbox.h mailmsg.h

mailbox_fromaddr.o:	mailbox_fromaddr.c mailbox.h mailmsg.h

mailmsg.o:		mailmsg.c mailmsg.h mailmsgmatenv.h

mailmsg_loadmb.o:	mailmsg_loadmb.c mailmsg.h mailmsgmatenv.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg_enver.h

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

mailmsghdrfold.o:	mailmsghdrfold.c mailmsghdrfold.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h

mailmsgstage.o:		mailmsgstage.c mailmsgstage.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsgfrom.o:		mailmsgfrom.c mailmsgfrom.h

realname.o:		realname.c realname.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

gecos.o:		gecos.c gecos.h

kinfo.o:		kinfo.c kinfo.h

msfile.o:		msfile.c msfile.h msfilee.h

msfile_best.o:		msfile_best.c msfile.h

msfile_best.o:		msfile_best.c msfile.h msfilee.h

msfilee.o:		msfilee.c msfilee.h

nodedb.o:		nodedb.c nodedb.h

clusterdb.o:		clusterdb.c clusterdb.h

kvsfile.o:		kvsfile.c kvsfile.h

fsdir.o:		fsdir.c fsdir.h

objfile.o:		objfile.c objfile.h

ipasswd.o:		ipasswd.c ipasswd.h

sysusers.o:		sysusers.c sysusers.h

sysgroups.o:		sysgroups.c sysgroups.h

sysprojects.o:		sysprojects.c sysprojects.h

sysusershells.o:	sysusershells.c sysusershells.h

syspasswd.o:		syspasswd.c syspasswd.h

sysshadow.o:		sysshadow.c sysshadow.h

sysgroup.o:		sysgroup.c sysgroup.h

sysproject.o:		sysproject.c sysproject.h

sysserattr.o:		sysuserattr.c sysuserattr.h

filebuf.o:		filebuf.c filebuf.h

filebuf_writeblanks.o:	filebuf_writeblanks.c filebuf.h

filebuf_writefill.o:	filebuf_writefill.c filebuf.h

filebuf_writealign.o:	filebuf_writealign.c filebuf.h

filebuf_writezero.o:	filebuf_writezero.c filebuf.h

filebuf_mailsupport.o:	filebuf_mailsupport.c filebuf.h

dirlist.o:		dirlist.c dirlist.h

pwi.o:			pwi.c pwi.h ipasswd.h

userattr.o:		userattr.c userattr.h

commandment.o:		commandment.c commandment.h commandments.h

biblebook.o:		biblebook.c biblebook.h biblebooks.h

bibleverse.o:		bibleverse.c bibleverse.h bibleverses.h

bvs.o:			bvs.c bvs.h

bvsmk.o:		bvsmk.c bvsmk.h

bvsbook.o:		bvsbook.c bvsbook.h

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

raqhand.o:		raqhand.c raqhand.h

namecache.o:		namecache.c namecache.h

gncache.o:		gncache.c gncache.h

address.o:		address.c address.h

babycalc.o:		babycalc.c babycalc.h

babiesfu.o:		babiesfu.c babiesfu.h

dater.o:		dater.c dater.h

dater_setkey.o:		dater_setkey.c dater.h

cvtdater.o:		cvtdater.c cvtdater.h

dayspec.o:		dayspec.c dayspec.h

tmz.o:			tmz.c tmz.h

sysvar.o:		sysvar.c sysvar.h

var.o:			var.c var.h

varmk.o:		varmk.c varmk.h

varhdr.o:		varhdr.c varhdr.h

mxalias.o:		mxalias.c mxalias.h

issue.o:		issue.c issue.h

motd.o:			motd.c motd.h

holidayer.o:		holidayer.c holidayer.h holidays.h

holidays.o:		holidays.c holidays.h

calyear.o:		calyear.c calyear.h

calyears.o:		calyears.c calyears.h

dayofmonth.o:		dayofmonth.c dayofmonth.h

ema.o:			ema.c ema.h

ema_first.o:		ema_first.c ema.h

outema.o:		outema.c outema.h

manstr.o:		manstr.c manstr.h

cyi.o:			cyi.c cyi.h cyifu.h

cyimk.o:		cyimk.c cyimk.h cyifu.h

cyimkwait.o:		cyimkwait.c cyifu.h

ebuf.o:			ebuf.c ebuf.h

msumsg.o:		msumsg.c msumsg.h

numincr.o:		numincr.c numincr.h

vars.o:			vars.c vars.h

grcache.o:		grcache.c grcache.h

hostinfo.o:		hostinfo.c hostinfo.h

tmpx.o:			tmpx.c tmpx.h

tmpx_getuserlines.o:	tmpx_getuserlines.c tmpx.h

sigblock.o:		sigblock.c sigblock.h

finduid.o:		finduid.c finduid.h

pwcache.o:		pwcache.c pwcache.h

dumper.o:		dumper.c dumper.h

cm.o:			cm.c cm.h

cmbuf.o:		cmbuf.c cmbuf.h

sysdialer.o:		sysdialer.c sysdialer.h

systems.o:		systems.c systems.h

logfile.o:		logfile.c logfile.h

logfile_userinfo.o:	logfile_userinfo.c logfile.h

logfile_printfold.o:	logfile_printfold.c logfile.h

hdrdecode.o:		hdrdecode.cc hdrdecode.h 
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) hdrdecode.cc

mailalias.o:		mailalias.cc mailalias.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) $<

termtrans.o:		termtrans.cc termtrans.h 
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) $<

termout.o:		termout.cc termout.h ansigr.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) termout.cc

linehist.o:		linehist.cc linehist.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) linehist.cc

langstate.o:		langstate.cc langstate.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) langstate.cc

hexdecoder.o:		hexdecoder.cc hexdecoder.h 
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) hexdecoder.cc

qpdecoder.o:		qpdecoder.cc qpdecoder.h char.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) qpdecoder.cc

b64decoder.o:		b64decoder.cc b64decoder.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) b64decoder.cc

utf8decoder.o:		utf8decoder.cc utf8decoder.h 
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) utf8decoder.cc

obuf.o:			obuf.cc obuf.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) obuf.cc

osetstr.o:		osetstr.c osetstr.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) osetstr.cc

osetint.o:		osetint.cc osetint.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) osetint.cc

setint.o:		setint.cc setint.h
	$(GPP) -c $(CCOPTS_GCCOPTSGPP) $(CFLAGS) $(CPPFLAGS) setint.o

chartrans.o:		chartrans.c chartrans.h 

mbcache.o:		mbcache.c mbcache.h

conslog.o:		conslog.c conslog.h

termnote.o:		termnote.c termnote.h

sysusernames.o:		sysusernames.c sysusernames.h

sysrealname.o:		sysrealname.c sysrealname.h

aclinfo.o:		aclinfo.c aclinfo.h

acltypes.o:		acltypes.c acltypes.h

snflags.o:		snflags.c snflags.h

spawner.o:		spawner.c spawner.h

rahand.o:		rahand.c rahand.h

msgdata.o:		msgdata.c msgdata.h

lookword.o:		lookword.c lookword.h

outstore.o:		outstore.c outstore.h

uiconv.o:		uiconv.c uiconv.h

varsub.o:		varsub.c varsub.h

cachetime.o:		cachetime.c cachetime.h

pdb.o:			pdb.c pdb.h


# BFILE

bopen.o:		bopen.c bfile.h

breadline.o:		breadline.cc bfile.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<


# testing

X01= testnodefs

X01OBJ= $(X01).o

X01OBJS= $(CRTFRONT) $(X01OBJ) $(CRTC) $(CRTBACK)

$(X01).o:		$(X01).c

$(X01).x:		$(X01OBJS)
	$(LD) -z nodefs -o $@ $(LDFLAGS) $(X01OBJS) $(XLIBINFO) -lfoo

$(X01).so:		$(X01OBJS)
	$(LD) -G -o $@ $(LDFLAGS) $(X01OBJS) $(XLIBINFO)


X02= testvalues

X02OBJ= $(X02).o $(VALUES)

X02OBJS= $(CRTFRONT) $(X02OBJ) $(CRTC) $(CRTBACK)

$(X02).o:		$(X02).c

$(X02).x:		$(X02OBJS)
	$(LD) -o $@ $(LDFLAGS) $(X02OBJS) $(XLIBINFO) > $(X02).lm


