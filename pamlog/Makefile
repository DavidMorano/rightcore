# MAKEFILE

T= pamlog

ALL= $(T).so

AM=
#AM=sparcv9

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man4
HELPDIR= $(SRCROOT)/share/help

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
#CCOPTS= -xO5 -K pic -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -K pic -xtarget=ultra -xsafe=mem -dalign -xdepend

# UltraSPARC-v9 (64 bits)
#CCOPTS= -xO5 -xtarget=ultra -xarch=v9 -xsafe=mem -dalign -xdepend


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

CFLAGS= $(CCOPTS) 

#LD= $(CC)

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR)/$(AM) -M $(T).map -B direct
#LDFLAGS= -m ${LF_LDFLAGS) -R$(LIBDIR) -M $(T).map 
LDFLAGS_R=


LIBDIRS= -L$(LIBDIR)/$(AM)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -Bstatic -luc -lu -Bdynamic
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


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
TOUCH= /usr/bin/touch


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


OBJ00= session.o
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

$(T).so:		$(OBJS) Makefile $(T).map
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

strip:			$(T).so
	strip $(T).so

install:		$(ALL) Makefile install.incs
	-mkdir -p $(LIBDIR)/ksh 2> /dev/null
	makenewer -r $(ALL) $(LIBDIR)/ksh/

install.test:		$(ALL) Makefile install.incs
	-mkdir -p $(LIBDIR)/ksh 2> /dev/null
	rm -f libkshc.so
	cp -p libkshb.so libkshc.so
	makenewer -r libkshc.so $(LIBDIR)/ksh/

install-help:		$(T).help
	makeinstall-help $(HELPDIR)

install.incs:		install.ih install.io
	$(TOUCH) $@

install.ih:		$(INSTALLINCS)

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
	makenewer -z -m o=h $(OBJI) -d $(INCDIR) -t $@

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

control:
	(uname -n ; date) > Control


obja.o:			$(OBJA)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJA) >> $(T).lm

objb.o:			$(OBJB)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJB) >> $(T).lm

objc.o:			$(OBJC)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJC) >> $(T).lm

objd.o:			$(OBJD)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJD) >> $(T).lm

obje.o:			$(OBJE)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJE) >> $(T).lm

objf.o:			$(OBJF)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJF) >> $(T).lm

objg.o:			$(OBJG)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJG) >> $(T).lm

objh.o:			$(OBJH)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJH) >> $(T).lm

obji.o:			$(OBJI)
	$(LD) -o $@ -r $(LDFLAGS_R) $(OBJI) >> $(T).lm


proginfo.o:		proginfo.c defs.h


sfreadline.o:		sfreadline.c


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

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


