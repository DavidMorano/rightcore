# MAKEFILE
# "Basic I/O" (BIO) library 'libb'

T= libb

ALL= $(T).a

SRCROOT= $(LOCAL)


INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ultra -xsafe=mem -dalign -xdepend

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

LDFLAGS=

LIBDIRS=

LIBS=

RANLIB= ranlib

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= bfile.h


OBJ00=
OBJ01= bopen.o bopentmp.o bopencmd.o bopenrcmd.o bopenlock.o
OBJ02= bread.o breadline.o breadlines.o breadlinetimed.o bgetc.o 
OBJ03= bwrite.o bputc.o bprintf.o bufprintf.o 
OBJ04= bcopy.o bcopyblock.o bcopyfile.o
OBJ05= bflush.o 
OBJ06= bcontrol.o btruncate.o
OBJ07= bseek.o btell.o bseek64.o btell64.o
OBJ08= bclose.o 
OBJ09= 
OBJ10= eprintf.o nprintf_r.o fdprintf.o
OBJ11=
#OBJ12= sprintf.o 
OBJ13=
OBJ14= 
OBJ15=
OBJ16= bfile_flush.o bfile_mktmpfile.o bfile_pagein.o
OBJ17= bfile_sfbasename.o bfile_strbasename.o
OBJ18= sbuf.o outbuf.o vecstr.o ctdec.o perm.o permsched.o
OBJ19= vstrkeycmp.o strkeycmp.o lockfile.o
OBJ20= mkdirs.o mktmpfile.o ids.o
OBJ21= sfdirname.o
OBJ22= format.o 
#OBJ23= econvert.o


OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= obja.o objb.o objc.o


.SUFFIXES:		.ls .i .cx .cs


all:			$(ALL) $(T).a(bprintf.o)
 
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

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T).a:			$(OBJ)
	$(AR) -cr $@ $?

$(T).so:		$(OBJS) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS)

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

install:		$(T).a
	makenewer $(T).a $(LIBDIR)/$(OFD)
	makenewer $(T).a $(LIBDIR)
	makenewer $(INCS) $(INCDIR)

install.$(OFF):		install

again:
	rm -f $(T).a

clean:			again
	rm -f $(T).a *.o

control:
	uname -n > Control
	date >> Control


obja.o:			$(OBJA)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJC)


$(T).a(bprintf.o):	bprintf.o


# special (very special) processing for NPRINTF
nprintf_r.o:		nprintf.o lockfile.o format.o nprintf.map Makefile
	$(LD) -r -o $@ -B reduce -M nprintf.map nprintf.o lockfile.o format.o


# special (very special) processing for EPRINTF
eprintf_r.o:		eprintf.o format.o eprintf.map Makefile
	$(LD) -r -o $@ -B reduce -M eprintf.map eprintf.o format.o


econvert.o:	econvert.obj
	rm -f econvert.o
	cp -p econvert.obj econvert.o


ids.o:		ids.c ids.h

bclose.o:		bclose.c bfile.h
bcontrol.o:		bcontrol.c bfile.h
bcopy.o:		bcopy.c bfile.h
bcopyblock.o:		bcopyblock.c bfile.h
bcopyfile.o:		bcopyfile.c bfile.h
bflush.o:		bflush.c bfile.h
bgetc.o:		bgetc.c bfile.h
bopen.o:		bopen.c bfile.h
bopencmd.o:		bopencmd.c bfile.h
bopenlock.o:		bopenlock.c bfile.h
bopenprog.o:		bopenprog.c bfile.h
bopenrcmd.o:		bopenrcmd.c bfile.h
bopenremote.o:		bopenremote.c bfile.h
bopensched.o:		bopensched.c bfile.h
bopentmp.o:		bopentmp.c bfile.h
bprintf.o:		bprintf.c bfile.h
bputc.o:		bputc.c bfile.h
bread.o:		bread.c bfile.h
breadline.o:		breadline.c bfile.h
breadlines.o:		breadlines.c bfile.h
breadlinetimed.o:	breadlinetimed.c bfile.h
bseek.o:		bseek.c bfile.h
btell.o:		btell.c bfile.h
btruncate.o:		btruncate.c bfile.h
bwasteline.o:		bwasteline.c bfile.h
bwrite.o:		bwrite.c bfile.h
 
btell64.o:		btell64.c bfile.h
bseek64.o:		bseek64.c bfile.h

bfile_flush.o:		bfile_flush.c bfile.h
bfile_mktmpfile.o:	bfile_mktmpfile.c bfile.h
bfile_pagein.o:		bfile_pagein.c bfile.h
bfile_sfbasename.o:	bfile_sfbasename.c bfile.h
bfile_strbasename.o:	bfile_strbasename.c bfile.h


