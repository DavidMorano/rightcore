# MAKEFILE

T= testksh

ALL= $(T).so

#SRCROOT= $(HOME)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs


# HyperSPARC
#CCOPTS= -xO5 -K pic -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -K pic -xtarget=ultra -xsafe=mem -xdepend


DEF00=
DEF01=
DEF02=
DEF03=
DEF04=
DEF05=
DEF06=
DEF07= $(LF_DEFS)

DEF08=
DEF09=
DEF10=
DEF11=
DEF12=
DEF13=
DEF14=
DEF15=

DEF16=
DEF17= -DKSHBUILTIN=1
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

ASTDEFS= $(DEFA) $(DEFC) 


INCDIRS= -I$(INCDIR)

ASTINCDIRS= -I$(AST)/include/ast -I$(INCDIR)
#ASTINCDIRS= -I/usr/include -I$(INCDIR) -I$(AST)/include/ast 

CPPFLAGS= $(DEFS) $(INCDIRS)

ASTCPPFLAGS= $(ASTDEFS) $(ASTINCDIRS)

CFLAGS= $(CCOPTS) 

ASTCFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= ld

LDFLAGS= -i $(LF_LDFLAGS) -R$(LIBDIR) -M $(T).map 
LDFLAGS_R= $(LF_LDFLAGS)

ASTLDFLAGS=


LIBDIRS= -L$(LIBDIR)

ASTLIBSDIR= -L$(AST)/lib

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

GNULIBS= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc

ASTLIBS= -lcmd -lshell -last $(GNULIBS)

SYSLIBS= -lpthread -lrt -lxnet -lsocket -lnsl -ldl

STDLIBS= -lc

SALIBS= $(SYSLIBS) $(ASTLIBS) $(STDLIBS)


CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

CRT0= $(CRT1) $(CRTI) $(VALUES)

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
TOUCH= /bin/touch


INSTALLINCS0= install.inca install.incb install.incc install.incd
INSTALLINCS1= install.ince install.incf

INSTALLINCS= $(INSTALLINC0) $(INSTALLINC1)


I00=
I01= defs.h
I02=
I03=
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

INCS= $(INCA) $(INCB) $(INCC) $(INCD)


OBJ00=
OBJ01= b_testwrite.o
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

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23) 
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)
OBJI= $(OBJ64) $(OBJ65) $(OBJ66) $(OBJ67) $(OBJ68) $(OBJ69) $(OBJ70) $(OBJ71)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) $(OBJI)

#OBJS= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o
#OBJS= obja.o objb.o objc.o
OBJS= obja.o 

ASTOBJS= obja.o


.SUFFIXES:		.ls .i .cx .cs


default:		$(ALL)


.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

# stand-alone LINT analysis
.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

# stand-alone cross reference
.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T).a:			$(OBJ)
	$(AR) -cr $(T).a $?

$(T).so:		$(OBJS) Makefile $(T).a $(T).map
	$(LD) -G -o $@ -m $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

strip:			$(T).so
	strip $(T).so

install:		$(ALL) Makefile 
	-mkdir -p $(LIBDIR)/ksh 2> /dev/null
	makenewer -r $(ALL) $(LIBDIR)/ksh/

install-test:		$(ALL) Makefile 
	-mkdir -p $(LIBDIR)/ksh 2> /dev/null
	rm -f libkshc.so
	cp -p libkshb.so libkshc.so
	makenewer -r libkshc.so $(LIBDIR)/ksh/

install-help:		$(T).help
	makeinstall-help $(HELPDIR)

other:			here
	makenewer -z -m o=h $(OBJA) -d $(INCDIR)
	makenewer -z -m o=h $(OBJB) -d $(INCDIR)
	makenewer -z -m o=h $(OBJC) -d $(INCDIR)
	makenewer -z -m o=h $(OBJD) -d $(INCDIR)
	makenewer -z -m o=h $(OBJE) -d $(INCDIR)
	makenewer -z -m o=h $(OBJF) -d $(INCDIR)
	makenewer -z -m o=h $(OBJG) -d $(INCDIR)
	makenewer -z -m o=h $(OBJH) -d $(INCDIR)
	makenewer -z -m o=h $(OBJI) -d $(INCDIR) -t $@

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

control:
	(uname -n ; date) > Control


obja.o:			$(OBJA) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJA) 

objb.o:			$(OBJB) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJB)

objc.o:			$(OBJC) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJC)

objd.o:			$(OBJD) Makefile
	$(LD) -r -o $@ $(LDFLAGS_R) $(OBJD)

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


b_la.o:			b_la.c b_la.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_look.o:		b_look.c
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_msu.o:		b_msu.c msu_config.h $(INCS) msfile.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_msinfo.o:		b_msinfo.c msinfo_config.h $(INCS) msfile.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_wn.o:			b_wn.c wn_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_rename.o:		b_rename.c rename_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_shcat.o:		b_shcat.c shcat_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_touch.o:		b_touch.c touch_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_biblebook.o:		b_biblebook.c biblebook_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_l.o:			b_l.c l_config.h $(INCS) fsdir.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_clustername.o:	b_clustername.c clustername_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_s.o:			b_s.c la_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_chacl.o:		b_chacl.c chacl_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_kshbi.o:		b_kshbi.c kshbi_config.h $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_helloworld.o:		b_helloworld.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testnull.o:		b_testnull.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testout.o:		b_testout.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testenv.o:		b_testenv.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testdir.o:		b_testdir.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testcat.o:		b_testcat.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testread.o:		b_testread.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testwrite.o:		b_testwrite.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

b_testsleep.o:		b_testsleep.c $(INCS)
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<


chacl_aux.o:		chacl_aux.c chacl_config.h $(INCS)


proginfo.o:		proginfo.c defs.h


getrootdname.o:		getrootdname.c

getclusters.o:		getclusters.c

termwritable.o:		termwritable.c

dater_setkey.o:		dater_setkey.c

msfile_best.o:		msfile_best.c


shio.o:			shio.c shio.h
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<

sfreadline.o:		sfreadline.c
	$(CC) -c $(ASTCFLAGS) $(ASTCPPFLAGS) $<


userinfo.o:		userinfo.c userinfo.h

ctdecpi.o:		ctdecpi.c


paramfile.o:		paramfile.c paramfile.h

char.o:			char.c char.h

field.o:		field.c field.h

field_srvarg.o:		field_srvarg.c field.h

fifostr.o:		fifostr.c fifostr.h

fifoelem.o:		fifoelem.c fifoelem.h

vecstr.o:		vecstr.c vecstr.h

vecelem.o:		vecelem.c vecelem.h

veclist.o:		vechand.c vechand.h

hdb.o:			hdb.c hdb.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

loadave.o:		loadave.c loadave.h

entropy.o:		entropy.c entropy.h

sha1.o:			sha1.c sha1.h

buffer.o:		buffer.c buffer.h

bufstr.o:		bufstr.c bufstr.h

sbuf.o:			sbuf.c sbuf.h

lookaside.o:		lookaside.c lookaside.h

filemap.o:		filemap.c filemap.h

q.o:			q.c q.h

plainq.o:		plainq.c plainq.h

pq.o:			pq.c pq.h

charq.o:		charq.c charq.h

serialbuf.o:		serialbuf.c serialbuf.h

netorder.o:		netorder.c netorder.h

storeitem.o:		storeitem.c storeitem.h

outbuf.o:		outbuf.c outbuf.h

matenv.o:		matenv.c matenv.h

msg.o:			msg.c msg.h matenv.h

dater.o:		dater.c dater.h

realname.o:		realname.c realname.h

inetaddr.o:		inetaddr.c inetaddr.h

gecos.o:		gecos.c gecos.h

kinfo.o:		kinfo.c kinfo.h

msfile.o:		msfile.c msfile.h msfilee.h

msfilee.o:		msfilee.c msfilee.h

msfile_best.o:		msfile_best.c msfile.h msfilee.h

lookaside.o:		lookaside.c lookaside.h

nodedb.o:		nodedb.c nodedb.h

clusterdb.o:		clusterdb.c clusterdb.h

kvsfile.o:		kvsfile.c kvsfile.h

fsdir.o:		fsdir.c fsdir.h

objfile.o:		objfile.c objfile.h



