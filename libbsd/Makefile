# MAKEFILE

T= libbsd

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)


GPPFLAGS= $(CGS_CCFLAGS)

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) -Wall
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing -Wall $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


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

XCFLAGS= $(LF_CFLAGS) $(CPPFLAGS)
CFLAGS= $(CCOPTS) $(XCFLAGS)


#LD= $(CC)


LDRPATH=

LDFLAGS= $(LF_LDFLAGS)


LIBDIRS= -L$(CGS_LIBDIR)

LIB0=
LIB1=
LIB2=
LIB3=
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl

LIBS= $(LF_LIBS) $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1=
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4=
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7=

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


LINT= lint
LINTFLAGS= -uxn -Dlint $(LF_LINTFLAGS) 

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /usr/bin/touch


INSTALLINC0= install-inca install-incb install-incc install-incd
INSTALLINC1= install-ince install-incf install-incg install-inch
INSTALLINC2= install-inci install-incj install-inck install-incl
INSTALLINC3= install-incm install-incn install-inco install-incp
INSTALLINC4= install-incq install-incr install-incs install-inct
INSTALLINC5= install-incu install-incv

INSTALLINCa= $(INSTALLINC0) $(INSTALLINC1) $(INSTALLINC2) $(INSTALLINC3)
INSTALLINCb= $(INSTALLINC4) $(INSTALLINC5)

INSTALLINCS= $(INSTALLINCa) $(INSTALLINCb)


I00= vsystem.h exitcodes.h localmisc.h
I01= vechand.h vecstr.h vecitem.h vecobj.h vecint.h veclong.h vecelem.h
I02= hdb.h lookaside.h pq.h plainq.h
I03= field.h 
I04= egs.h randmwc.o random.h
I05= dater.h zdb.h zos.h tmz.h zoffparts.h
I06= termstr.h
I07= kinfo.h loadave.h fsdir.h fsdirtree.h
I08= logfile.h lfm.h lastlogfile.h tmpx.h
I09= storeitem.h 
I10= mallocstuff.h userinfo.h openport.h
I11= char.h ansigr.h baops.h bwops.h
I12= envs.h
I13= raqhand.h
I14=
I15=

INCA= $(I00) $(I01) $(I02) $(I03)
INCB= $(I04) $(I05) $(I06) $(I07)
INCC= $(I08) $(I09) $(I10) $(I11)
INCD= $(I12) $(I13) $(I14) $(I15)
INCE= $(I16) $(I17) $(I18) $(I19) 
INCF= $(I20) $(I21) $(I22) $(I32)

INCS= $(INCA) $(INCB) $(INCC) $(INCD) $(INCE) $(INCF) 


OBJ00= bsd.o
OBJ01=
OBJ02=
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

OBJ56=
OBJ57=
OBJ58=
OBJ59=
OBJ60=
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

OBJ144=
OBJ145=
OBJ146=
OBJ147=
OBJ148=
OBJ149=
OBJ150=
OBJ151=

OBJ152=
OBJ153=
OBJ154=
OBJ155=
OBJ156=
OBJ157=
OBJ158=
OBJ159=

OBJ160=
OBJ161=
OBJ162=
OBJ163=
OBJ164=
OBJ165=
OBJ166=
OBJ167=

OBJ168=
OBJ169=
OBJ170=
OBJ171=
OBJ172=
OBJ173=
OBJ174=
OBJ175=

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

OBJN0= $(OBJ104) $(OBJ105) $(OBJ106) $(OBJ107) 
OBJN1= $(OBJ108) $(OBJ109) $(OBJ110) $(OBJ111)
OBJN= $(OBJN0) $(OBJN1)

OBJO0= $(OBJ112) $(OBJ113) $(OBJ114) $(OBJ115) 
OBJO1= $(OBJ116) $(OBJ117) $(OBJ118) $(OBJ119)
OBJO= $(OBJO0) $(OBJO1)

OBJP0= $(OBJ120) $(OBJ121) $(OBJ122) $(OBJ123) 
OBJP1= $(OBJ124) $(OBJ125) $(OBJ126) $(OBJ127)
OBJP= $(OBJP0) $(OBJP1)

OBJQ0= $(OBJ128) $(OBJ129) $(OBJ130) $(OBJ131) 
OBJQ1= $(OBJ132) $(OBJ133) $(OBJ134) $(OBJ135)
OBJQ= $(OBJQ0) $(OBJQ1)

OBJR0= $(OBJ136) $(OBJ137) $(OBJ138) $(OBJ139) 
OBJR1= $(OBJ140) $(OBJ141) $(OBJ142) $(OBJ143)
OBJR= $(OBJR0) $(OBJR1)

OBJS0= $(OBJ144) $(OBJ145) $(OBJ146) $(OBJ147) 
OBJS1= $(OBJ148) $(OBJ149) $(OBJ150) $(OBJ151)
OBJS= $(OBJS0) $(OBJS1)

OBJT0= $(OBJ152) $(OBJ153) $(OBJ154) $(OBJ155) 
OBJT1= $(OBJ156) $(OBJ157) $(OBJ158) $(OBJ159)
OBJT= $(OBJT0) $(OBJT1)

OBJU0= $(OBJ160) $(OBJ161) $(OBJ162) $(OBJ163) 
OBJU1= $(OBJ164) $(OBJ165) $(OBJ166) $(OBJ167)
OBJU= $(OBJU0) $(OBJU1)

OBJV0= $(OBJ168) $(OBJ169) $(OBJ170) $(OBJ171) 
OBJV1= $(OBJ172) $(OBJ173) $(OBJ174) $(OBJ175)
OBJV= $(OBJV0) $(OBJV1)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) \
	$(OBJE) $(OBJF) $(OBJG) $(OBJH) \
	$(OBJI) $(OBJJ) $(OBJK) $(OBJL) \
	$(OBJM) $(OBJN) $(OBJO) $(OBJP) \
	$(OBJQ) $(OBJR) $(OBJS) $(OBJT) $(OBJU) $(OBJV)

OBJFILE0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o 
OBJFILE1= obji.o objj.o objk.o objl.o objm.o objn.o objo.o objp.o 
OBJFILE2= objq.o objr.o objs.o objt.o obju.o objv.o

OBJFILE= $(OBJFILE0) $(OBJFILE1) $(OBJFILE2)


.SUFFIXES:		.ls .i .cx .cs


default:		belowincs install-inc all
#default:		all

all:			below $(ALL) 
#all:			$(ALL) 

.c.o:
	$(CC) -c $(CFLAGS) $<

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(T).a:			$(OBJ)
	$(AR) -cr $(T).a $?

$(T).so:		$(OBJFILE) Makefile localmisc.h $(T).a
	$(LD) -G -o $@ $(LDFLAGS) $(OBJFILE) $(LIBDIRS) $(LIBS)

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
	makesafe -v=3 -I $(INCDIR) $(OBJN)
	makesafe -v=3 -I $(INCDIR) $(OBJO)
	makesafe -v=3 -I $(INCDIR) $(OBJP)
	makesafe -v=3 -I $(INCDIR) $(OBJQ)
	makesafe -v=3 -I $(INCDIR) $(OBJR)
	makesafe -v=3 -I $(INCDIR) $(OBJS)
	makesafe -v=3 -I $(INCDIR) $(OBJT)
	makesafe -v=3 -I $(INCDIR) $(OBJU)
	makesafe -v=3 -I $(INCDIR) $(OBJV)

install-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		install-inc Makefile $(ALL)
	makenewer -r $(ALL) -d $(SRCROOT)/lib

install-inc:		install-ih install-io
	touch install-inc

install-ih:		$(INSTALLINCS)
	touch install-ih

install-inca:		$(INCA)
	makenewer -z $? -d $(INCDIR) -t $@

install-incb:		$(INCB)
	makenewer -z $? -d $(INCDIR) -t $@

install-incc:		$(INCC)
	makenewer -z $? -d $(INCDIR) -t $@

install-incd:		$(INCD)
	makenewer -z $? -d $(INCDIR) -t $@

install-ince:		$(INCE)
	makenewer -z $? -d $(INCDIR) -t $@

install-incf:		$(INCF)
	makenewer -z $? -d $(INCDIR) -t $@

install-incg:		$(INCG)
	makenewer -z $? -d $(INCDIR) -t $@

install-inch:		$(INCH)
	makenewer -z $? -d $(INCDIR) -t $@

install-inci:		$(INCI)
	makenewer -z $? -d $(INCDIR) -t $@
install-incj:		$(INCJ)
	makenewer -z $? -d $(INCDIR) -t $@
install-inck:		$(INCK)
	makenewer -z $? -d $(INCDIR) -t $@
install-incl:		$(INCL)
	makenewer -z $? -d $(INCDIR) -t $@
install-incm:		$(INCM)
	makenewer -z $? -d $(INCDIR) -t $@
install-incn:		$(INCN)
	makenewer -z $? -d $(INCDIR) -t $@
install-inco:		$(INCO)
	makenewer -z $? -d $(INCDIR) -t $@
install-incp:		$(INCP)
	makenewer -z $? -d $(INCDIR) -t $@
install-incq:		$(INCQ)
	makenewer -z $? -d $(INCDIR) -t $@
install-incr:		$(INCR)
	makenewer -z $? -d $(INCDIR) -t $@
install-incs:		$(INCS)
	makenewer -z $? -d $(INCDIR) -t $@
install-inct:		$(INCT)
	makenewer -z $? -d $(INCDIR) -t $@
install-incu:		$(INCU)
	makenewer -z $? -d $(INCDIR) -t $@
install-incv:		$(INCV)
	makenewer -z $? -d $(INCDIR) -t $@

install-io:		$(OBJ)
	makenewer -im -z -m o=h $? -d $(INCDIR) -t $@

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
	makenewer -im -z -m o=h $(OBJJ) -d $(INCDIR) -t $@
	makenewer -im -z -m o=h $(OBJK) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJL) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJM) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJN) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJO) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJP) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJQ) -d $(INCDIR)

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

objs.o:			$(OBJS)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJS)

objt.o:			$(OBJT)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJT)

obju.o:			$(OBJU)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJU)

objv.o:			$(OBJV)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJV)


field.o:		field.c field.h

field_word.o:		field_word.c field.h

field_srvarg.o:		field_srvarg.c field.h

dstr.o:			dstr.c dstr.h

fifostr.o:		fifostr.c fifostr.h

fifoitem.o:		fifoitem.c fifoitem.h

fifoelem.o:		fifoelem.c fifoelem.h

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_foilcmp.o:	vecstr_foilcmp.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_envfile.o:	vecstr_envfile.c vecstr.h

vecpstr.o:		vecpstr.c vecpstr.h

vecpstr_loadfile.o:	vecpstr_loadfile.c vecpstr.h

vecpstr_loadgrusers.o:	vecpstr_loadgrusers.c vecpstr.h

vecpstr_loadpjusers.o:	vecpstr_loadpjusers.c vecpstr.h

vecpstr_envadd.o:	vecpstr_envadd.c vecpstr.h

vecpstr_envfile.o:	vecpstr_envfile.c vecpstr.h

vecitem.o:		vecitem.c vecitem.h

vecobj.o:		vecobj.c vecobj.h

vecint.o:		vecint.c vecint.h

veclong.o:		veclong.c veclong.h

vecelem.o:		vecelem.c vecelem.h

hdb.o:			hdb.c hdb.h

hdbstr.o:		hdbstr.c hdbstr.h

keyvals.o:		keyvals.c keyvals.h

paramopt.o:		paramopt.c paramopt.h

logfile.o:		logfile.c logfile.h

logfile_userinfo.o:	logfile_userinfo.c logfile.h userinfo.h

logfile_printfold.o:	logfile_printfold.c logfile.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

loadave.o:		loadave.c loadave.h

egs.o:			egs.c egs.h

sha1.o:			sha1.c sha1.h

buffer.o:		buffer.c buffer.h

buffer_blanks.o:	buffer_blanks.c buffer.h

buffer_extras.o:	buffer_extras.c buffer.h

buffer_stropaque.o:	buffer_stropaque.c buffer.h

buffer_strcompact.o:	buffer_strcompact.c buffer.h

buffer_strquote.o:	buffer_strquote.c buffer.h

sbuf.o:			sbuf.c sbuf.h

sbuf_termconseq.o:	sbuf_termconseq.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

sbuf_blanks.o:		sbuf_blanks.c sbuf.h

bufstr.o:		bufstr.c bufstr.h

outstore.o:		outstore.c outstore.h

strmgr.o:		strmgr.c strmgr.h

lookaside.o:		lookaside.c lookaside.h

filemap.o:		filemap.c filemap.h

q.o:			q.c q.h

plainq.o:		plainq.c plainq.h

pq.o:			pq.c pq.h

cpq.o:			cpq.c cpq.h

charq.o:		charq.c charq.h

aiq.o:			aiq.c aiq.h

cq.o:			cq.c cq.h

piq.o:			piq.c piq.h

ciq.o:			ciq.c ciq.h

serialbuf.o:		serialbuf.c serialbuf.h

netorder.o:		netorder.c netorder.h

storeitem.o:		storeitem.c storeitem.h

outbuf.o:		outbuf.c outbuf.h

matenv.o:		matenv.c matenv.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsgmathdr.o:	mailmsgmathdr.c mailmsgmathdr.h

mailmsgfrom.o:		mailmsgfrom.c mailmsgfrom.h

mailbox.o:		mailbox.c mailbox.h

mailbox_getfrom.o:	mailbox_getfrom.c mailbox.h

mailmsg.o:		mailmsg.c mailmsg.h mailmsgmatenv.h

mailmsg_loadfile.o:	mailmsg_loadfile.c mailmsg.h

mailmsg_loadfd.o:	mailmsg_loadfile.c mailmsg.h

mailmsg_loadmb.o:	mailmsg_loadmb.c mailmsg.h mailmsgmatenv.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg.h mailmsgmatenv.h

mailmsg_envtimes.o:	mailmsg_envtimes.c mailmsg.h

mailmsg_envdates.o:	mailmsg_envdates.c mailmsg.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h mailmsg.h

mailmsgstage.o:		mailmsgstage.c mailmsgstage.h 

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

mailmsghdrfold.o:	mailmsghdrfold.c mailmsghdrfold.h

staackaddr.o:		stackaddr.c stsackaddr.h

mailaddrquote.o:	mailaddrquote.c mailaddrquote.h

char.o:			char.c char.h

comparse.o:		comparse.c comparse.h

mhcom.o:		mhcom.c mhcom.h

dater.o:		dater.c dater.h

dater_getdate.o:	dater_getdate.c dater.h date.h

date.o:			date.c date.h

zdb.o:			zdb.c zdb.h

zos.o:			zos.c zos.h

tmz.o:			tmz.c tmz.h

zoffparts.o:		zoffparts.c zoffparts.h

realname.o:		realname.c realname.h

gecos.o:		gecos.c gecos.h

kinfo.o:		kinfo.c kinfo.h

nulstr.o:		nulstr.c nulstr.h

dupstr.o:		dupstr.c dupstr.h

msfile.o:		msfile.c msfile.h msfilee.h

msfilee.o:		msfilee.c msfilee.h

ebuf.o:			ebuf.c ebuf.h

mapstrint.o:		mapstrint.c mapstrint.h

srvreg.o:		srvreg.c srvreg.h srvrege.h

srvrege.o:		srvrege.c srvrege.h

spawnproc.o:		spawnproc.c spawnproc.h

openport.o:		openport.c openport.h

openportmsg.o:		openportmsg.c openportmsg.h

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

mailalias.o:		mailalias.cc mailalias.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) mailalias.cc

filebuf.o:		filebuf.c filebuf.h

filebuf_writeblanks.o:	filebuf_writeblanks.c filebuf.h

filebuf_writefill.o:	filebuf_writefill.c filebuf.h

filebuf_writealign.o:	filebuf_writealign.c filebuf.h

filebuf_writezero.o:	filebuf_writezero.c filebuf.h

filebuf_writefd.o:	filebuf_writefd.c filebuf.h 

filebuf_mailsupport.o:	filebuf_mailsupport.c filebuf.h 

memfile.o:		memfile.c memfile.h

baops.o:		baops.c baops.h

bits.o:			bits.c bits.h baops.h

msgenv.o:		msgenv.c msgenv.h

rijndael.o:		rijndael.c rijndael.h

ema.o:			ema.c ema.h

mimetypes.o:		mimetypes.c mimetypes.h

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

pwfile.o:		pwfile.c pwfile.h pwentry.h

ipasswd.o:		ipasswd.c ipasswd.h

wordfill.o:		wordfill.c wordfill.h

linefold.o:		linefold.c linefold.h

tmstrs.o:		tmstrs.c tmstrs.h

rmermsg.o:		rmermsg.c rmermsg.h

modload.o:		modload.c modload.h

dirseen.o:		dirseen.c dirseen.h

dirseen_not.o:		dirseen_not.c dirseen.h

logsys.o:		logsys.c logsys.h

sysmiscem.o:		sysmiscem.c sysmiscem.h

sysmiscfh.o:		sysmiscfh.c sysmiscfh.h

csem.o:			csem.c csem.h

lockrw.o:		lockrw.c lockrw.h

termnote.o:		termnote.c termnote.h

dirlist.o:		dirlist.c dirlist.h

finduid.o:		finduid.c finduid.h

pwcache.o:		pwcache.c pwcache.h

grcache.o:		grcache.c grcache.h

gncache.o:		gncache.c gncache.h

namecache.o:		namecache.c namecache.h

useraccdb.o:		useraccdb.c useraccdb.h

grmems.o:		grmems.c grmems.h

termtrans.o:		termtrans.cc termtrans.h 
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) termtrans.cc

termout.o:		termout.cc termout.h ansigr.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) termout.cc

hdrdecode.o:		hdrdecode.cc hdrdecode.h 
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) hdrdecode.cc

hdrdecode2.o:		hdrdecode2.cc hdrdecode2.h 
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) hdrdecode2.cc

qpdecoder.o:		qpdecoder.cc qpdecoder.h char.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) qpdecoder.cc

b64decoder.o:		b64decoder.cc b64decoder.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) b64decoder.cc

hexdecoder.o:		hexdecoder.cc hexdecoder.h 
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) hexdecoder.cc

utf8decoder.o:		utf8decoder.cc utf8decoder.h 
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) utf8decoder.cc

obuf.o:			obuf.cc obuf.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) obuf.cc

setint.o:		setint.cc setint.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) setint.cc

osetint.o:		osetint.cc osetint.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) osetint.cc

osetstr.o:		osetstr.cc osetstr.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) osetstr.cc

setstr.o:		setstr.c setstr.h

mapstrs.o:		mapstrs.c mapstrs.h

vsetstr.o:		vsetstr.c vsetstr.h

setstr.o:		setstr.c setstr.h

chartrans.o:		chartrans.c chartrans.h 

uiconv.o:		uiconv.c uiconv.h 

tmtime.o:		tmtime.c tmtime.h

tmpx.o:			tmpx.c tmpx.h

tmpx_getrunlevel.o:	tmpx_getrunlevel.c tmpx.h

tmpx_getuserlines.o:	tmpx_getuserlines.c tmpx.h

tmpx_getsessions.o:	tmpx_getsessions.c tmpx.h

envs.o:			envs.c envs.h

envs_procxe.o:		envs_procxe.c envs.h

envs_subs.o:		envs_subs.c envs.h

pwi.o:			pwi.c pwi.h ipasswd.h

thrcomm.o:		thrcomm.c thrcomm.h

thrbase.o:		thrbase.c thrbase.h

filecounts.o:		filecounts.c filecounts.h

expcook.o:		expcook.c expcook.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h

cachetime.o:		cachetime.c cachetime.h


ids.o:			ids.c ids.h

dayspec.o:		dayspec.c dayspec.h

bcspec.o:		bcspec.c bcspec.h


getax.o:		getax.c getax.h

getpe.o:		getpe.c getpe.h

gethe.o:		gethe.c gethe.h

getse.o:		getse.c getse.h

getne.o:		getne.c getne.h

getus.o:		getus.c getus.h

getua.o:		getua.c getua.h


getdefzinfo.o:		getdefzinfo.c getdefzinfo.h

snflags.o:		snflags.c snflags.h

fsdirtreestat.o:	fsdirtreestat.c fsdirtree.h

userinfo.o:		userinfo.c userinfo.h storeitem.h char.h gecos.h

getutmpent.o:		getutmpent.c getutmpent.h

getutmpterm.o:		getutmpterm.c getutmpent.h

getsysmisc.o:		getsysmisc.c getsysmisc.h

shellunder.o:		shellunder.c shellunder.h

udomain.o:		udomain.c

cpuspeed.o:		cpuspeed.c

ncpu.o:			ncpu.c 

rfile.o:		rfile.c incfile_rfilewrite.h

mkuiname.o:		mkuiname.c userinfo.h

mkuibang.o:		mkuibang.c userinfo.h

newobj.o:		newobj.c newobj.h

emainfo.o:		emainfo.c emainfo.h

sysmemutil.o:		sysmemutil.c sysmemutil.h

address.o:		address.c address.h


incfile_rfilewrite.h:	rfilewrite
	mkincfile rfilewrite

pwentry.o:		pwentry.c pwentry.h

getpwentry.o:		getpwentry.c getpwentry.h pwentry.h

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

mkuuid.o:		mkuuid.c mkuuid.h

snfsflags.o:		snfsflags.c snflags.h

snopenflags.o:		snopenflags.c snflags.h

snpollflags.o:		snpollflags.c snflags.h

snfilemode.o:		snfilemode.c snflags.h

sntmtime.o:		sntmtime.c tmtime.h

snmkuuid.o:		snmkuuid.c snmkuuid.h mkuuid.h

calstrs.o:		calstrs.c calstrs.h

findbit.o:		findbit.c findbit.h

hasallunique.o:		hasallunique.cc
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) hasallunique.cc


unlinkd.o:		$(*)/$(@)
	cd $(*) ; make up

nifinfo.o:		$(*)/$(@)
	cd $(*) ; make up

belowincs:
	makebelow upincs

below:
	makebelow


