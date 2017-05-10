# MAKEFILE


T= userhome


#SRCROOT= $(HOME)

INCDIR= $(SRCROOT)/include
BINDIR= $(SRCROOT)/bin
LIBDIR= $(SRCROOT)/lib
HELPDIR= $(SRCROOT)/share/help


CC= gcc
GPP= g++

#CCOPTS_GCCALL= -pthreads -fexceptions
#CCOPTS_CCALL= -MT

#CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
#CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

#CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs


INCDIRS= -I$(INCDIR)

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

CPPFLAGS= $(DEFS) $(INCDIRS) $(LF_CPPFLAGS)

CFLAGS= $(CCOPTS) $(LF_CFLAGS)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -lff
LIB2=
LIB3=
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


OBJ00= main.o gecosname.o
OBJ01= b_$(T).o
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=

OBJ08= vecstr_env.o vecstr_adduniq.o vecstr.o
OBJ09= filebuf.o filebuf_writeblanks.o
OBJ10= bits.o field.o
OBJ11= vechand.o
OBJ12=
OBJ13=
OBJ14=
OBJ15=

OBJ16= sncpy.o snsd.o 
OBJ17= snwcpy.o
OBJ18= 
OBJ19= cfdec.o cfbin.o cfoct.o cfhex.o cfnum.o ctdec.o 
OBJ20= mkpath.o mknpath.o
OBJ21= storebuf.o toxc.o char.o
OBJ22= sfbasename.o sfdirname.o
OBJ23= findbit.o

OBJ24= strnlen.o strlinelen.o
OBJ25= strwcpy.o
OBJ26= strnpbrk.o strnchr.o strnrchr.o
OBJ27= bufprintf.o format.o
OBJ28= optbool.o optvalue.o
OBJ29= matstr.o matostr.o matocasestr.o matkeystr.o
OBJ30= strkeycmp.o vstrkeycmp.o
OBJ31= nleadcasestr.o nleadstr.o nleadkeystr.o

OBJ32= hasprintbad.o hasallbase.o hasalldig.o
OBJ33= isprintbad.o isdigitlatin.o isprintlatin.o isalphalatin.o
OBJ34=
OBJ35= checkbase.o nextpowtwo.o
OBJ36=
OBJ37= isNotPresent.o
OBJ38= getourenv.o
OBJ39= debugprintf.o

OBJ40= uc_memalloc.o uc_strtobase.o uc_confstr.o
#OBJ41= uc_open.o uc_opendialer.o uc_openfint.o uc_openfs.o uc_openfsvc.o
#OBJ42= uc_openinfo.o uc_openpass.o uc_openprog.o uc_openproto.o uc_openpsem.o
#OBJ43= uc_openpt.o uc_openshm.o uc_opensocket.o
#OBJ44= uc_openuser.o
OBJ45= uc_reade.o uc_writen.o uc_nonblock.o uc_moveup.o
OBJ46= 
OBJ47= 

OBJ48= 
OBJ49= uc_fork.o uc_atfork.o 
OBJ50= uc_atexit.o uc_exit.o
OBJ51= uc_execve.o
OBJ52= 
OBJ53=
OBJ54= ptm.o ptma.o ptc.o ptca.o lockrw.o ids.o
OBJ55= msleep.o

OBJ56= u_open.o u_poll.o u_close.o
OBJ57= u_read.o u_write.o u_seek.o u_tell.o u_seekoff.o 
OBJ58= u_fcntl.o u_dup.o u_dup2.o u_stat.o u_fstat.o u_pipe.o
OBJ59= u_mapfile.o u_munmap.o
OBJ60= u_fork.o u_execve.o u_fchmod.o
OBJ61=
OBJ62=
OBJ63=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH)

OBJS= $(OBJ)


SRC= $(OBJ:.c=.o)


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


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

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


$(T).x:			$(OBJ) Makefile
	$(CC) -o $@ $(OBJS) $(LIBDIRS) $(LIBS)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x

clean:
	rm -f *.o *.i *.x

again:
	rm -f $(T).x


main.o:			main.c
	$(CC) -c -o $@ $(CCOPTS) $(CPPFLAGS) main.c 

b_$(T).o:		b_$(T).c b_$(T).h
	$(CC) -c -o $@ $(CCOPTS) $(CPPFLAGS) b_$(T).c 

gecosname.o:		gecosname.c

main.i:			main.c
	cpp -std=c89 $(CPPFLAGS) main.c > main.i


vecstr.o:		vecstr.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

bits.o:			bits.c bits.h

field.o:		field.c field.h

ptm.o:			ptm.c ptm.h

ptma.o:			ptma.c ptma.h

ptc.o:			ptc.c ptc.h

ptca.o:			ptca.c ptca.h

lockrw.o:		lockrw.c lockrw.h

ids.o:			ids.c ids.h


sncpy.o:		sncpy.c sncpy.h

snwcpy.o:		snwcpy.c snwcpy.h

mkpath.o:		mkpath.c mkpath.h

mknpath.o:		mknpath.c mknpath.h

char.o:			char.c char.h

toxc.o:			toxc.c toxc.h

cfdec.o:		cfdec.c cfdec.h

cfbin.o:		cfbin.c cfbin.h

cfoct.o:		cfoct.c cfoct.h

cfhex.o:		cfhex.c cfhex.h

cfnum.o:		cfnum.c cfnum.h

ctdec.o:		ctdec.c ctdec.h

findbit.o:		findbit.c findbit.h

storebuf.o:		storebuf.c storebuf.h

format.o:		format.c format.h

spawnproc.o:		spawnproc.c spawnproc.h



